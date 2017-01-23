#include "conveyor.h"
/* Конвейер:
 * концепция разбития исполнения инструкций кода на этапы (извлечение инструкции, ее декодирование,
 * извлечение операндов, вычисление инструкции и запись результата в память (кэш)) с последующим возможным одновременным
 * исполнением различных этапов - например, во время обсчета инструкции на АЛУ возможен запрос следующей команды
 * и так далее.
 *
 * Цель конвейера - оптимизировать утилизацию ресурсов процессора и шины; например, если есть большое количество АЛУ
 * и мы хотим загрузить всех их работой, узким местом является шина - АЛУ не хватает инструкций и операндов для того,
 * чтобы приступить к работе.
 *
 * Здесь написана симуляция конвейера - его деятельность не отражается прямо на симуляции ПДП, скорее просто ведется подсчет,
 * насколько работа конвейера "помогла" оптимизации - за какое среднем число "тактов" исполняется программа с применением технологии
 * конвейера.
 *
 * Глоссарий:
 * - конвейер: концепция разбития исполнения инструкций на этапы и последовательно-параллельное исполнения оных этапов
 * - модель конвейера: то, как в этом продукте отслеживается работа конвейера. Включает в себя еще модели инструкций.
 * - инструкция: то, что исполняется на VCPU.
 * - модель инструкции: то, как видит модель конвейера инструкцию; именно с этим модель конвейера и взаимодействует. Заметим, что нас
 *                      (модель конвейера) интересует лишь некоторые параметры инструкции; инструкция упрощается до некоторой ее модели (да ну)
 * - объект конвейера/объект модели конвейера: объект, которым реализована в этом продукте модель конвейера.
 * - объект модели инструкции: объект, с которым взаимодействует объект конвейера.
 * - такт: квант времени процессора; с точки зрения процессора, время течет не непрерывно, а тактами.
 * - модель такта: такт с точки зрения модели конвейера. Он же "такт". Такое разделение возникает из-за того, что такты симуляции вообще говоря
 *                 никак не связаны с тактами конвейера. Нас же интересует, сколько именно моделируемых тактов в среднем занимает исполнение.
 *
 * Как устроена симуляция конвейера: этот класс отслеживает запросы к памяти на чтение/запись и запросы на исполнение инструкций на АЛУ
 * посредством вызовов соответствующих методов конвейера. С помощью этой информации он поддерживает модель конвейера.
 *
 * Модель конвейера: предполагаем, что в нашей системе (на плате, на кристалле, в чипе) есть n ALU и одна шина. Ограничим также количество
 * инструкций, с которыми одновременно работает конвейер до m. Каждая инструкция должна пройти пять этапов конвейера (с некоторыми исключениями,
 * для которых некоторые этапы пропускаются). Исполнение каждого этапа каждой инструкции занимает определенное число "тактов" - именно усреднение
 * этой величины мы и будем сравнивать при наличии конвейера (m отлично от 1) и без оного (1 "лента" конвейера - при этом и АЛУ, по существу, один).
 *
 * Проследим работу конвейера с инструкцией и из этого выведем подходящую реализацию его модели. Инструкция поступает в модель в момент извлечения
 * кода инструкции по адресу PC из памяти. Соответственно, в этот момент конвейер должен создавать объект, соответствующий этой инструкции, заполнять
 * поля этого объекта и говорить, что первый этап конвейера для этой инструкции исполнен. При записи в память результата исполнения инструкции конвейер
 * должен удалять эту инструкцию из своей модели.
 *
 * Рассмотрим теперь состояние модели - на конвейере несколько инструкций, возможно на различных этапах. Будем симулировать работу конвейера "прыжками
 * во времени" - в объекте конвейера поддерживается текущее число тактов и при попытке продвижения он ищет самую ближайшую точку во времени (такт),
 * когда произойдет перемена состояния конвейера - то есть, какая-то инструкция перейдет на следующий этап/освободит нить конвейера.
 *
 * Получаем, что конвейер должен уметь быстро находить следующую точку прыжка во времени (скорость не критична, но неплохо бы). Соответственно,
 * 1) из объекта инструкции в модели мы должны уметь понять, на каком она этапе, когда этот этап начался и когда он закончится (или сколько он длится.
 *    Эту информацию будет предоставлять VCPU при вызове методов конвейера на добавление команды)
 *
 * 2) инструкции в модели должны храниться так, чтобы мы более-менее оптимально могли найти нужный скачок. Предполагая в нашей модели строгую последовательность,
 *    можно сказать, что одинаковые этапы различных инструкций выполняются в порядке их следования в коде. Такая структура намекает нам на следующую систему:
 *    для каждого этапа конвейера мы поддерживаем очередь. При вычислении следующего скачка мы смотрим в голову каждой очереди и ищем ближайший такт скачка.
 *    Производя скачок, мы попаем инструкцию из данного этапа и пушим ее в очередь следующего этапа, записывая в объект модели инструкции такт, когда
 *    это произошло.
 *
 * 2.5) некоторые инструкции не такие, как все - с точки зрения конвейера, им не нужны некоторые этапы. Это мы будем отслеживать с помощью дополнительной
 *    информации в объекте модели инструкции. Эта дополнительная информация будет влиять на то, как именно команда продвигается по конвейеру при скачках.
 *
 * 2.75) при попытке скачка может оказаться, что следующая команда хочет зафетчить область памяти, куда должна написать, но еще не написала предыдущая
 *    инструкция конвейера. Чтобы это отслеживать, модель конвейера должна знать, от каких областей памяти зависит данный этап данной инструкции.
 *    Соответственно, если конвейер замечает, что он собирался протолкнуть по конвейеру инструкцию, которая зависит от еще неизмененных участков памяти,
 *    то вместо этого он пытается протолкнуть что-то другое - получается, что данная инструкция задерживается на конвейере; возникает отложенное продвижение.
 *    Как это реализовать (без неебических затрат по памяти - то есть, не отмечать в каждой модели ячейки кто с ней работает)? Можно поддеривать еще одну
 *    очередь, в которую складывать модели инструкций и указывать их зависимости/области действия. Тогда при каждом скачке проверять с помощью этой очереди,
 *    не пытаемся ли мы продвинуть нечто, что зависит от еще необсчитанного/незанесенного в память. (<- DISCUSS-RESOLED: лучше поддерживать массив
 *    задействованных участков памяти, в котором будут храниться зависимые инструкции. Порядок в инструкций коде будем фиксировать счетчиком внутри
 *    объекта модели инструкции - при int64 их хватит с головой. Тогда при попытке продвижения инструкции заглядываем по участкам памяти зависимости и ищем
 *    )
 *
 * 3) вопрос: когда и на сколько должен продвигаться конвейер? Ответ: в моменты вызова метода добавления команды конвейер должен произвести такое количество
 *    скачков, чтобы свежая команда встала на конвейер.
 *
 * 4) из работы VCPU с операндами для нас представляет интерес лишь структура операндов - сколько происходит разыменований и обращений к памяти (шине)
 *    на соответствующих этапах. Информация об операндах поступает в другом вызове нежели добавление команды. (DISCUSS-RESOLVED: или поступает в том же? То
 *    есть, можно это делать получестно - VCPU фетчит инструкцию и операнды и только затем говорит конвейеру че да как. В такой реализации VCPU заполняет
 *    промежуточную структуру/объект, которую и передаст затем конвейеру. ОТВЕТ: лучше делать одним вызовом, чтобы сам конвейер был устроен проще и не
 *    редактировал жизненно важную инфу на лету)
 *
 * 5) заметим, что при продвижении выбирается ближайшая точка завершения этапа некоторой инструкции; при этом, если мы выбрали продвигать
 *    этап, зависимый от шины, то одновременно продвигается этап, зависимый от АЛУ - и наоборот.
 *
 * 6) в нашей модели также нужно отслеживать занятость АЛУ - поскольку перемена состояния конвейера может происходить посредине исполнения инструкции
 *    на АЛУ, мы должны гарантировать, что при рассчете прыжка те инструкции, которые вычислялись, все еще будут вычисляться. Собственно, сделаем это
 *    в лоб: после того, как мы вычислили длину прыжка, в первую очередь продвинем по конвейеру те инструкции, у которых уже есть прогресс на АЛУ;
 *    только затем продвинем все остальные. (DISCUSS: норм подход?)
 *
 * Из вышесказанного запишем то, как же будет выглядеть класс конвейера.
 *
 * */
conveyor::conveyor()
{

}

uint64_t conveyor::add_instruction(instr_model *instr)
{
    instr->instr_num = instr_counter_;
    instr->conv_phase = 0;
    instr_counter_++;
    bool can_insert_instr = false; //we can insert new instruction if every instruction on conveyor has passed first phase; otherwise
                                   //we have to make advancements until no instruction is in first phase and we have less than cap number
                                   //of instructions
    while (!can_insert_instr)
    {
        can_insert_instr = true;
        for (int i = 0; i < (int) conv_model_.size(); i++)
            if (conv_model_[i]->conv_phase == 0)
            {
                can_insert_instr = false;
                break;
            }
        if (can_insert_instr && (conv_model_.size() < instruction_cap_))
            break;
        advance();
    }
    conv_model_.push_back(instr);
    return instr_counter_;
}

void conveyor::advance()
{
    if (conv_model_.empty())
        return;

    instr_model * hyp_instr = conv_model_[0]; //hypothetical instruction to advance; we try with instruction awaiting it's fetch

    int alu_occupied = 0;
    bool bus_occupied = false;

    for (int i = 0; i < (int) conv_model_.size(); i++)
        if (conv_model_[i]->conv_phase == 3 && conv_model_[i]->curr_phase_advance != 0)
            alu_occupied++;

    for (int i = 0; i < (int) conv_model_.size(); i++)
    {//looking for a closer jump point

        if ((conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] - conv_model_[i]->curr_phase_advance) <
            (hyp_instr->ticks_per_phase[hyp_instr->conv_phase] - hyp_instr->curr_phase_advance))
        {//jump point is closer in time; gotta check memory collisions and that there is a free ALU if we want to start to calculate
            bool no_collision = true;

            if (conv_model_[i]->conv_phase == 3 && conv_model_[i]->curr_phase_advance == 0 && alu_occupied >= alu_num_)
                no_collision = false;

            if (conv_model_[i]->conv_phase == 4)
            {// collision is possible only on write-back

                for (int j = 0; j < (int) conv_model_.size(); j++)
                {

                    if (conv_model_[j]->instr_num >= conv_model_[i]->instr_num)
                        continue; //instruction happens after our chosen; no collision
                    //collision: we try to write into area from which previous instruction reads; lost causality

                    for (int ii = 0; ii < (int) conv_model_[i]->dependencies_out.size(); ii++)
                    {

                        for (int jj = 0; jj < (int) conv_model_[j]->dependencies_in.size(); jj++)
                        {

                            if (conv_model_[i]->dependencies_out[ii] == conv_model_[j]->dependencies_in[jj])
                            {// yup, that's a collision
                                no_collision = false;
                                break;
                            }
                        }

                        if (!no_collision)
                            break;
                    }
                    if (!no_collision)
                        break;
                }
            }

            if (no_collision)
            {//so, our jump point is closer and no collisions are present and there is free ALU if we need one
                hyp_instr = conv_model_[i];
            }
        }
    }
    //so, we found closest jump point - closest event in our conveyor model. It's time to advance our conveyor accordingly
    //our instruction is first to switch to the next phase; therefore, all other instruction will advance inside their phases
    //(considering bus and ALU occupation)

    int advance_ticks = hyp_instr->ticks_per_phase[hyp_instr->conv_phase] - hyp_instr->curr_phase_advance;
    //number of ticks by which conveyor advances

    cur_ticks_ += (uint64_t) advance_ticks; //track advancement

    for (int i = 0; i < (int)conv_model_.size(); i++)
        conv_model_[i]->has_advanced = false; //no instruction has advanced yet


    for (int i = 0; i < (int)conv_model_.size(); i++)
    {//we look at our instructions and try to advance their execution by advance_ticks;  first we try unfinished instructions

        if (conv_model_[i]->curr_phase_advance == 0) //if execution of phase has not began, we advance it in next iteration
            continue;

        //first we occupy bus if necessary and if possible
        if (conv_model_[i]->conv_phase == 0 || conv_model_[i]->conv_phase == 2 || conv_model_[i]->conv_phase == 4)
        {
            if (!bus_occupied)
                bus_occupied = true; //if we have to use bus and its free - occupy it
            else
            {//if bus is occupied - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }

        //occupy one ALU if necessary
        if (conv_model_[i]->conv_phase == 3)
        {
            if (alu_occupied < alu_num_)
                alu_occupied ++; //if we have to use ALU and one is free - occupy it
            else
            {//no free ALU - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }

        conv_model_[i]->curr_phase_advance += advance_ticks;

        if (conv_model_[i]->curr_phase_advance >= conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase])
        {//if current phase is finished - transfer to the next one
            conv_model_[i]->conv_phase++;
            while(conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] == 0)
                conv_model_[i]->conv_phase++;
            conv_model_[i]->curr_phase_advance = 0;

            if (conv_model_[i]->conv_phase > 4)
            {//we advanced past last phase - pop instruction from conveyor and free occupied by support structures memory
                free(conv_model_[i]);
                conv_model_.erase(conv_model_.begin() + i);
            }
        }
        conv_model_[i]->has_advanced = true;
    }

    for (int i = 0; i < (int)conv_model_.size(); i++)
    {
        if (conv_model_[i]->has_advanced)
            continue;
        if (conv_model_[i]->conv_phase == 0 || conv_model_[i]->conv_phase == 2 || conv_model_[i]->conv_phase == 4)
        {
            if (!bus_occupied)
                bus_occupied = true; //if we have to use bus and its free - occupy it
            else
            {//if bus is occupied - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }
         //occupy one ALU if necessary
        if (conv_model_[i]->conv_phase == 3)
        {
            if (alu_occupied < alu_num_)
                alu_occupied ++; //if we have to use ALU and one is free - occupy it
            else
            {//no free ALU - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }
        conv_model_[i]->curr_phase_advance += advance_ticks;

        if (conv_model_[i]->curr_phase_advance >= conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase])
        {//if current phase is finished - transfer to the next one
            conv_model_[i]->conv_phase++;
            while(conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] == 0)
                conv_model_[i]->conv_phase++;

            conv_model_[i]->curr_phase_advance = 0;

            if (conv_model_[i]->conv_phase > 4)
            {//we advanced past last phase - pop instruction from conveyor and free occupied by support structures memory
                free(conv_model_[i]);
                conv_model_.erase(conv_model_.begin() + i);
            }
        }
        conv_model_[i]->has_advanced = true;
    }

}
uint64_t conveyor::get_ticks()
{
    return cur_ticks_;
}
