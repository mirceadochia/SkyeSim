#ifndef DEFAULTAIRCRAFTLOCATOR_HXX
#define DEFAULTAIRCRAFTLOCATOR_HXX

#include <string>
#include <simgear/misc/sg_path.hxx>

#include <Main/AircraftDirVisitorBase.hxx>

#include <QAbstractListModel>

namespace flightgear
{

std::string defaultAirportICAO();

/**
 * we don't want to rely on the main AircraftModel threaded scan, to find the
 * default aircraft, so we do a synchronous scan here, on the assumption that
 * FG_DATA/Aircraft only contains a handful of entries.
 */
class DefaultAircraftLocator : public AircraftDirVistorBase
{
public:
    DefaultAircraftLocator();

    SGPath foundPath() const;

private:
    virtual VisitResult visit(const SGPath& p) override;

    std::string _aircraftId;
    SGPath _foundPath;
};

class WeatherScenariosModel : public QAbstractListModel
{
    Q_OBJECT
public:
    WeatherScenariosModel(QObject* pr = nullptr);

    int rowCount(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString metarForItem(int index) const;

    Q_INVOKABLE QString nameForItem(int index) const;

    Q_INVOKABLE QString descriptionForItem(int index) const;
private:
    struct WeatherScenario
    {
        QString name;
        QString description;
        QString metar;
    };

    std::vector<WeatherScenario> m_scenarios;

    enum {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        MetarRole
    };
};

}

#endif // DEFAULTAIRCRAFTLOCATOR_HXX
